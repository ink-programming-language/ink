// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var type_cpp: dynamic = cpp_uninitialized();
      var second_info: dynamic = cpp_uninitialized();
      read(type_cpp, second_info);
      var min: dynamic = INT_MAX;
      var __cpp_switch_1: dynamic = type_cpp;
      if (__cpp_switch_1 == 0)
      {
        write(queues[(second_info - 1)].front(), cpp_char("\n"));
        queues[(second_info - 1)].pop();
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        {
        var j: dynamic = 0;
        while ((j < queues.size()))
        {
        min = min(min, cpp_cast(queues[j].size()));
        j += 1;
        }
        }
        for (var queue: dynamic in queues)
        {
        if ((queue.size() == min))
        {
        queue.push(second_info);
        break;
        }
        }
        break;
      }
      else
      {
        return 1;
      }
      i += 1;
    }
  }
}
