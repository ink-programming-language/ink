// Translated from solution.cpp.

var N_MAX = 10;

func main()
{
  var n: dynamic;
  var nn: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var kk: dynamic;
  var t: dynamic;
  var res = 0;
  var k = cpp_array(N_MAX);
  var s = cpp_array(N_MAX, N_MAX);
  var p = cpp_array(N_MAX);
  var ss = cpp_array(N_MAX);
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
