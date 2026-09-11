// Translated from solution.cpp.

var N_MAX: dynamic = 10;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var nn: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var kk: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
  var k: dynamic = cpp_array(N_MAX);
  var s: dynamic = cpp_array(N_MAX, N_MAX);
  var p: dynamic = cpp_array(N_MAX);
  var ss: dynamic = cpp_array(N_MAX);
  read(n, m);
  {
    i = 0;
    while ((i < m))
    {
      read(k[i]);
      {
        j = 0;
        while ((j < k[i]))
        {
          read(s[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < m))
    {
      read(p[i]);
      i += 1;
    }
  }
  nn = (1 << n);
  {
    i = 0;
    while ((i < nn))
    {
      {
        j = 0;
        while ((j < n))
        {
          ss[j] = (((i >> j)) % 2);
          j += 1;
        }
      }
      {
        j = 0;
        while ((j < m))
        {
          t = 0;
          {
            kk = 0;
            while ((kk < k[j]))
            {
              if ((ss[(s[j][kk] - 1)] == 1))
              {
                t += 1;
              }
              kk += 1;
            }
          }
          if (((t % 2) != p[j]))
          {
            break;
          }
          j += 1;
        }
      }
      if ((j == m))
      {
        res += 1;
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
