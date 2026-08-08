// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      var type_cpp: dynamic;
      var second_info: dynamic;
      read(type_cpp, second_info);
      var min = INT_MAX;
      var __cpp_switch_1 = type_cpp;
      if (__cpp_switch_1 == 0)
      {
        write(queues[(second_info - 1)].front(), cpp_char("\n"));
        queues[(second_info - 1)].pop();
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        {
        var j = 0;
        while ((j < queues.size()))
        {
        min = min(min, cpp_cast(queues[j].size()));
        j += 1;
        }
        }
        for (var queue in queues)
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
        throw 0;
      }
      i += 1;
    }
  }
}
