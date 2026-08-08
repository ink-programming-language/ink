// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var max_r = -1;
  var cnt = 0;
  var max_c = -1;
  var min_r = 100000;
  var min_c = 100000;
  read(n, m);
  var x: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(x);
          if ((x == cpp_char("B")))
          {
            cnt += 1;
            max_c = max(max_c, j);
            min_c = min(min_c, j);
            max_r = max(max_r, i);
            min_r = min(min_r, i);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((max_r == -1))
  {
    write(1, "\n");
    return 0;
  }
  var c: dynamic;
  var r: dynamic;
  var len = 0;
  c = ((max_c - min_c) + 1);
  r = ((max_r - min_r) + 1);
  len = max(c, r);
  if ((cnt == 0))
  {
    write(1, "\n");
  } else if (((len > n) || (len > m)))
  {
    write(-1, "\n");
  } else
  {
    write(abs(((len * len) - cnt)), "\n");
  }
}
