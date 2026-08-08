// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var num: dynamic;
  read(n);
  while (1)
  {
    if ((n == 0))
    {
      break;
    }
    var vec1: dynamic;
    var vec2: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        read(num);
        vec1.push_back(num);
        read(num);
        vec2.push_back(num);
        i += 1;
      }
    }
    var x: dynamic;
    var flag = 1;
    var w = 0;
    {
      var j = 0;
      while ((j < vec1.size()))
      {
        var min = 9999999;
        {
          var i = 0;
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
