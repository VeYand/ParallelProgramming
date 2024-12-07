using System.Diagnostics;

namespace lab7_2;

internal static class Program
{
    private const string Url = "https://dog.ceo/api/breeds/image/random";
    private const int NumberOfRequests = 50; // Specify the number of requests here

    private static async Task Main()
    {
        SyncRequests();
        await AsyncRequests();
    }

    private static void SyncRequests()
    {
        using var client = new HttpClient();
        var stopwatch = Stopwatch.StartNew();

        for (var i = 0; i < NumberOfRequests; i++)
        {
            var response = client.GetAsync(Url).Result;
            if (!response.IsSuccessStatusCode)
            {
                continue;
            }

            var responseString = response.Content.ReadAsStringAsync().Result;
            Console.WriteLine(responseString);
        }

        stopwatch.Stop();
        Console.WriteLine($"Synchronous requests completed in: {stopwatch.ElapsedMilliseconds} ms");
    }

    private static async Task AsyncRequests()
    {
        using var client = new HttpClient();
        var stopwatch = Stopwatch.StartNew();
        
        var tasks = new Task<HttpResponseMessage>[NumberOfRequests];

        for (var i = 0; i < NumberOfRequests; i++)
        {
            tasks[i] = client.GetAsync(Url);
        }

        var responses = await Task.WhenAll(tasks);

        foreach (var response in responses)
        {
            if (!response.IsSuccessStatusCode)
            {
                continue;
            }

            var responseString = await response.Content.ReadAsStringAsync();
            Console.WriteLine(responseString);
        }

        stopwatch.Stop();
        Console.WriteLine($"Asynchronous requests completed in: {stopwatch.ElapsedMilliseconds} ms");
    }
}