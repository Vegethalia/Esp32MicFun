using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using System.Text.Json;
using MySqlConnector;

namespace Vegethalia
{
  public class ConsultaConsumElectricitat
  {
    /// <summary>
    /// Aquest enum serveix de shorcut per valors comuns. Realment no cal.
    /// </summary>
    enum AgruparPer { UN_MINUT = 1, DOS_MINUTS = 2, CINC_MINUTS = 5, TRENTA_MINUTS = 30, UNA_HORA = 60, DOS_HORES = 120, QUATRE_HORES = 240 };
    private readonly ILogger _logger;
    MySqlConnection? _conn = null;
    //string myConnectionString;

    public ConsultaConsumElectricitat(ILoggerFactory loggerFactory)
    {
      _logger = loggerFactory.CreateLogger<ConsultaConsumElectricitat>();

      try
      {
        var builder = new MySqlConnectionStringBuilder
        {
          Server = $"{Environment.GetEnvironmentVariable("MySqlServer")}",
          UserID = $"{Environment.GetEnvironmentVariable("MySqlUser")}",
          Password = $"{Environment.GetEnvironmentVariable("MySqlPass")}",
          Database = $"{Environment.GetEnvironmentVariable("MySqlDB")}",
          Port = UInt32.Parse(Environment.GetEnvironmentVariable("MySqlPort") ?? "3306")
          //        , SslMode = MySqlSslMode.Required
          //       , SslCert = @"G:\Trabajos\ESP32\Keys\EnergyDB_Vegethalia_key.pem"
          //, ConnectionTimeout = 10000
        };

        _logger.LogInformation($"Fent servir la connection string: [{builder.ConnectionString}]");
        _conn = new MySqlConnection(builder.ConnectionString);

        _conn.Open();
        _logger.LogInformation($"Connected to MySql Server [{_conn.ServerVersion}]!!");
      }
      catch (Exception ex)
      {
        _logger.LogError($"Cannot connect to MySql server: {ex.Message}");
      }
    }

    [Function("ConsultaConsumElectricitat")]
    public HttpResponseData Run([HttpTrigger(AuthorizationLevel.Function, "get", "post")] HttpRequestData req)
    {
      bool wrongRequest = (req.Query["dataFi"] == null) || (req.Query["numValors"] == null)
      || (req.Query["maxKWh"] == null) || (req.Query["mapejarDe0a"] == null)
      || (req.Query["agruparPer"] == null);
      if (wrongRequest)
      {
        var wrongResponse = req.CreateResponse(HttpStatusCode.BadRequest);
        wrongResponse.Headers.Add("Content-Type", "application/json");
        wrongResponse.WriteString("{\"MissatgeError\":\"Falten Paràmetres\"}");
        _logger.LogError($"Petició txunga! Falten paràmetres! NumParams={req.Query.Count}");
        return wrongResponse;
      }

      ReturnData theData = new ReturnData();
      var response = req.CreateResponse(HttpStatusCode.OK);
      response.Headers.Add("Content-Type", "application/json");

      try
      {
        int agruparPerMinuts = int.Parse(req.Query["agruparPer"] ?? "1");
        int numValors = int.Parse(req.Query["numValors"] ?? "64");
        double maxKWh = double.Parse(req.Query["maxKWh"] ?? "3.0");
        uint mapejarDe0a = uint.Parse(req.Query["mapejarDe0a"] ?? "20");
        DateTimeOffset dataFiUTC = DateTimeOffset.FromUnixTimeSeconds(uint.Parse(req.Query["dataFi"] ?? "0"));
        DateTimeOffset dataFi, dataIni;

        TimeZoneInfo tzi = TimeZoneInfo.FindSystemTimeZoneById("Central European Standard Time");
        dataFi = TimeZoneInfo.ConvertTimeFromUtc(dataFiUTC.UtcDateTime, tzi);
        if (dataFi.Year < 2010)
        {
          dataFi = DateTime.Now;
        }
        dataIni = dataFi.AddMinutes(0.0 - (numValors * agruparPerMinuts));

        _logger.LogInformation($"Processant petició: Valors={numValors} AgruparPer={agruparPerMinuts} MaxKWh={maxKWh} DataIni={dataIni.ToString()}  DataFin={dataFi.ToString()}");


        int numElems = 0;
        if (_conn is not null && _conn.State == System.Data.ConnectionState.Open)
        {
          using var command = _conn.CreateCommand();
          command.CommandText = $"SELECT DateReading, Intensity*0.230 AS KWh FROM CurrentReadings WHERE DateReading>=\"{dataIni.ToString("yyyy-MM-dd HH:mm:ss")}\" AND DateReading<=\"{dataFi.ToString("yyyy-MM-dd HH:mm:ss")}\"";
          _logger.LogInformation($"SQL Query --> [{command.CommandText}]");
          using var reader = command.ExecuteReader();
          List<ReturnData.EnergyElement> minuteReadings = new List<ReturnData.EnergyElement>();
          while (reader.Read())
          {
            minuteReadings.Add(new ReturnData.EnergyElement { TheDateFromEpoch = new DateTimeOffset(reader.GetDateTime(0)).ToUnixTimeSeconds(), TheValueInKWh = reader.GetFloat(1) });
            numElems++;
          }

          //i ara agrupem els valors
          double currentValue = 0.0;
          int currentIndex = 0;
          DateTimeOffset nextTime = dataIni;
          while (dataIni <= dataFi && currentIndex < minuteReadings.Count)
          {
            nextTime = dataIni.AddMinutes(agruparPerMinuts);
            double sumkwh = 0.0;
            int numItems = 0;
            while (currentIndex < minuteReadings.Count && minuteReadings[currentIndex].TheDateFromEpoch <= nextTime.ToUnixTimeSeconds())
            {
              sumkwh += minuteReadings[currentIndex].TheValueInKWh;
              numItems++;
              currentIndex++;
            }
            if (numItems > 0)
            {
              sumkwh /= numItems;
            }
            //sumkwh = Math.Min(sumkwh, maxKWh);
            //var valorMapped = (mapejarDe0a*sumkwh)/maxKWh; //regla de 3 per mapejar sumkwh to 0..mapejarDe0a
            theData.TheEnergyPoints.Add(new ReturnData.EnergyElement { TheDateFromEpoch = dataIni.ToUnixTimeSeconds(), TheValueInKWh = sumkwh });
            dataIni = nextTime.DateTime;
          }
          //omplenem lo que falti
          while (theData.TheEnergyPoints.Count < numValors)
          {
            nextTime = nextTime.AddMinutes(agruparPerMinuts);
            theData.TheEnergyPoints.Add(new ReturnData.EnergyElement { TheDateFromEpoch = 0, TheValueInKWh = 0 });
          }
        }

        theData.CommonInfo = $"NumElemsFromDB={numElems}";
        //theData.TheEnergyPoints.Add(new ReturnData.EnergyElement { TheDate = DateTimeOffset.Now, TheValueInKWh = 6996 });
      }
      catch (Exception ex)
      {
        _logger.LogError($"Error processant la petició: {ex.Message}");
      }

      string theJsonReturned = JsonSerializer.Serialize(theData);
      response.WriteString(theJsonReturned);

      return response;
    }
  }

  public class ReturnData
  {
    public struct EnergyElement
    {
      //public DateTimeOffset TheDate { get; set; }
      public long TheDateFromEpoch { get; set; }
      public double TheValueInKWh { get; set; }
    }
    public string CommonInfo { get; set; } = "";
    public List<EnergyElement> TheEnergyPoints { get; set; }

    public ReturnData()
    {
      TheEnergyPoints = new List<EnergyElement>();
    }
  }
}
