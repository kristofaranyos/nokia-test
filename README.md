## SimpleLogger

My solution for NokiaLoggingTask. It's a C++ based, header only logging framework.

**Requirements:**
 - C++11
 - Nothing else, just include the header, and it's ready for use

**Usage:**  
Include the header wherever you want to use it.
 Everything is in the SL namespace. Consider `using namespace SL` if your project is small.

Simply create a `Logger` instance. Prefer to use the empty constructor (it sets sensible default values for production environment), then use the fluent interface to specify what you want. See example.cpp.

To log something, simply use the log() function. The first parameter is the log type (`LogLevel::DEBUG`, `INFO`, `WARNING` and `ERROR`). The second is the message, and the third optional parameter is an error class, which extends std::exception. The error class' `what()` method is used to retrieve info from it.

You can retrieve the current log entries by calling `getErrors()` and delete an entry by calling `clear(log_id)`. It also throws an `InvalidLogIDException` when you try to clear a nonexistent one. You can start writing to a new log file using `setLogFull()`.

Console and file logging can be enabled independently. You can set the minimum log level, format, outputs (console stream, filename formats), logs/file, file rotation number (how many files are used to save the logs) and the date format. These all have setters, which are fairly straightforward. The exceptions:

 - `setFileName()`: Just embed [n] anywhere in the file name where you want to put the numbering. Example: `"log[n].txt"` would output `log1.txt`, `log2.txt`, etc
 - `setDateFormat()`: This uses `std::put_time` format, check the the [c++ reference](https://en.cppreference.com/w/cpp/io/manip/put_time)
 - `setConsoleFormat()` and `setFileFormat()`: The parameter is a lowercase string. You can write anything, and the tokens will be replaced with log data. Example: `"[T]: L, message: M"` would output something like `"[date]: ERROR, message: you forgot how to quit vim"`  
 Tokens:
     - I: Id
     - T: Time
     - L: LogLevel
     - M: Message
     - W: Output of `what()` if any

That's it, happy logging.

**Possible future features**
- Import preexisting log entries from files
- Make clear() delete entries from files as well
- Make lib thread safe
- Additional constructors with commonly required default settings (e.g. "development", etc)
- Better token parsing to permit uppercase characters in the format
