// Translated from solution.cpp.

var sq = (40 * 1000);

var used = cpp_array((sq + 1));

var G = [0, 1, 2, 1, 4, 3, 2, 1, 5, 6, 2, 1, 8, 7, 5, 9, 8, 7, 3, 4, 7, 4, 2, 1, 10, 9, 3, 6, 11, 12];

func main()
{
  var n: dynamic;
  read(n);
  var res = 0;
  var after_sq = 0;
  {
    var i = 1;
    while ((i <= sq))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      var cnt = (i == 1);
      {
        var j = i;
        while (((i != 1) && (j <= n)))
        {
          if ((j <= sq))
          {
            used[j] = true;
          } else
          {
            after_sq += 1;
          }
          cnt += 1;
          j *= i;
        }
      }
      res ^= G[cnt];
      i += 1;
    }
  }
  res ^= ((if ((n > sq)) (((n - sq) - after_sq)) else 0) & 1);
  if (res)
  {
    write("Vasya\n");
  } else
  {
    write("Petya\n");
  }
  return 0;
}
