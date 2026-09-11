// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var max_r: dynamic = -1;
  var cnt: dynamic = 0;
  var max_c: dynamic = -1;
  var min_r: dynamic = 100000;
  var min_c: dynamic = 100000;
  read(n, m);
  var x: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var len: dynamic = 0;
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
