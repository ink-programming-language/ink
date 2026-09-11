// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#i");

func main() -> dynamic
{
  while (true)
  {
    var N: dynamic = cpp_uninitialized();
    var K: dynamic = cpp_uninitialized();
    var tile: dynamic = cpp_array((MAX + 2), (MAX + 2));
    read(N, K);
    if (((N == 0) && (K == 0)))
    {
      break;
    }
    if (((K > ((1 << ((N / 2))))) || ((N % 2) == 1)))
    {
      write("No\n", "\n");
      continue;
    }
    K -= 1;
    {
      var j: dynamic = (N - 1);
      while ((j >= 0))
      {
        tile[1][(j + 1)] = ((K & ((1 << (((((N - 1) - j)) / 2))))));
        tile[0][(j + 1)] = (!tile[1][(j + 1)]);
        j -= 1;
      }
    }
    tile[1][0] = (!tile[1][1]);
    tile[1][(N + 1)] = (!tile[1][N]);
    {
      var i: dynamic = 1;
      while ((i < N))
      {
        {
          var j: dynamic = 0;
          while ((j < N))
          {
            var L: dynamic = tile[i][j];
            var C: dynamic = tile[i][(j + 1)];
            var R: dynamic = tile[i][(j + 2)];
            var T: dynamic = tile[(i - 1)][(j + 1)];
            tile[(i + 1)][(j + 1)] = ((((((((((((!T) && (!L)) && (!C)) && R)) || (((((!T) && (!L)) && C) && (!R)))) || (((((!T) && (!L)) && C) && R))) || (((((!T) && L) && (!C)) && (!R)))) || (((((!T) && L) && C) && (!R)))) || ((((T && (!L)) && (!C)) && (!R)))) || ((((T && (!L)) && C) && (!R)))));
            j += 1;
          }
        }
        tile[(i + 1)][0] = (!tile[(i + 1)][1]);
        tile[(i + 1)][(N + 1)] = (!tile[(i + 1)][N]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        {
          var j: dynamic = 0;
          while ((j < N))
          {
            write(( (tile[(i + 1)][(j + 1)]) ? cpp_char("E") : cpp_char(".")));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
