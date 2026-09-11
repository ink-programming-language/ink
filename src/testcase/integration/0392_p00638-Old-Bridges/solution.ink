// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  read(n);
  while (1)
  {
    if ((n == 0))
    {
      break;
    }
    var vec1: dynamic = cpp_uninitialized();
    var vec2: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(num);
        vec1.push_back(num);
        read(num);
        vec2.push_back(num);
        i += 1;
      }
    }
    var x: dynamic = cpp_uninitialized();
    var flag: dynamic = 1;
    var w: dynamic = 0;
    {
      var j: dynamic = 0;
      while ((j < vec1.size()))
      {
        var min: dynamic = 9999999;
        {
          var i: dynamic = 0;
          while ((i < vec2.size()))
          {
            if (((vec2[i] != -1) && (min > vec2[i])))
            {
              min = vec2[i];
              x = i;
            }
            i += 1;
          }
        }
        w += vec1[x];
        if ((w > vec2[x]))
        {
          flag = 0;
          break;
        }
        vec2[x] = -1;
        j += 1;
      }
    }
    if ((flag == 1))
    {
      write("Yes", "\n");
    } else
    {
      write("No", "\n");
    }
    read(n);
  }
  return 0;
}
