namespace lab7;

internal static class Program
{
    private static async Task Main()
    {
        Console.Write("Введите путь к текстовому файлу: ");
        var filePath = Console.ReadLine();

        Console.Write("Введите символы для удаления (без пробелов): ");
        var charsToRemove = Console.ReadLine();

        try
        {
            var content = await File.ReadAllTextAsync(filePath ?? "");

            var result = new string(content.Where(c => !charsToRemove?.Contains(c) ?? true).ToArray());

            await File.WriteAllTextAsync(filePath ?? "", result);

            Console.WriteLine("Символы успешно удалены и файл сохранен.");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Произошла ошибка: {ex.Message}");
        }
    }
}