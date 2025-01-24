using System.Net.Http;
using System.Threading.Tasks;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;

public static class TxuclaTalarnData
{
  private static readonly HttpClient httpClient = new HttpClient();

  [FunctionName("TxuclaTalarnData")]
  public static async Task Run([TimerTrigger("0 0 */12 * * *")] TimerInfo myTimer, ILogger log)
  {
    log.LogInformation($"TxuclaTalarnData function executed at: {DateTime.Now}");

    string apiUrl = "https://your-api-endpoint.com/api/data";
    HttpResponseMessage response = await httpClient.GetAsync(apiUrl);

    if (response.IsSuccessStatusCode)
    {
      string responseData = await response.Content.ReadAsStringAsync();
      log.LogInformation($"API Response: {responseData}");
    }
    else
    {
      log.LogError($"API call failed with status code: {response.StatusCode}");
    }
  }
}