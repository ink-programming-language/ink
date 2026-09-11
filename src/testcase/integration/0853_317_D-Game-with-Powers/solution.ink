// Translated from solution.cpp.

var sq: dynamic = (40 * 1000);

var used: dynamic = cpp_array((sq + 1));

var G: dynamic = [0, 1, 2, 1, 4, 3, 2, 1, 5, 6, 2, 1, 8, 7, 5, 9, 8, 7, 3, 4, 7, 4, 2, 1, 10, 9, 3, 6, 11, 12];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var res: dynamic = 0;
  var after_sq: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= sq))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      var cnt: dynamic = (i == 1);
      {
        var j: dynamic = i;
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
  res ^= (( ((n > sq)) ? (((n - sq) - after_sq)) : 0) & 1);
  if (res)
  {
    write("Vasya\n");
  } else
  {
    write("Petya\n");
  }
  return 0;
}
