// Translated from solution.cpp.

var inf = 1e9;

var eps = 1e-9;

var INF = inf;

var EPS = eps;

var dx = [1, -1, 0, 0];

var dy = [0, 0, 1, -1];

var N = cpp_array(((1100 * 1100) * 4), 2);

var T = cpp_array(110000);

var V = cpp_array(110000);

func main()
{
  var x: dynamic;
  var y: dynamic;
  var n: dynamic;
  var q: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var bb: dynamic;
  var it: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var nx: dynamic;
  var ny: dynamic;
  var ch: dynamic;
  scanf("%d%d", (&n), (&b));
  {
    i = 0;
    while ((i <= b))
    {
      {
        j = 0;
        while ((j <= b))
        {
          {
            k = 0;
            while ((k < 4))
            {
              nx = (i + dx[k]);
              ny = (j + dy[k]);
              nx = max(nx, 0);
              nx = min(nx, b);
              ny = max(ny, 0);
              ny = min(ny, b);
              N[0][(((((i * ((b + 1))) + j)) * 4) + k)] = (((((nx * ((b + 1))) + ny)) * 4) + k);
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d%d%d%d", (&a), (&bb), (&c), (&d));
      assert(((a != c) || (bb != d)));
      var len = (abs((a - c)) + abs((bb - d)));
      {
        it = 0;
        while ((it < 4))
        {
          if (((c == (a + (len * dx[it]))) && (d == (bb + (len * dy[it])))))
          {
            {
              j = 0;
              while ((j <= len))
              {
                x = (a + (j * dx[it]));
                y = (bb + (j * dy[it]));
                var id = ((((x * ((b + 1))) + y)) * 4);
                {
                  k = 0;
                  while ((k < 4))
                  {
                    N[0][(id + k)] = N[0][(id + it)];
                    k += 1;
                  }
                }
                j += 1;
              }
            }
            break;
          }
          it += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%d", (&q));
  {
    i = 0;
    while ((i < q))
    {
      scanf("%d%d %c %I64d", (&x), (&y), (&ch), (&T[i]));
      V[i] = ((((x * ((b + 1))) + y)) * 4);
      if ((ch == cpp_char("L")))
      {
        V[i] += 1;
      }
      if ((ch == cpp_char("U")))
      {
        V[i] += 2;
      }
      if ((ch == cpp_char("D")))
      {
        V[i] += 3;
      }
      i += 1;
    }
  }
  {
    it = 0;
    while ((it < 55))
    {
      {
        i = 0;
        while ((i < q))
        {
          if ((T[i] & ((1 << it))))
          {
            V[i] = N[0][V[i]];
          }
          i += 1;
        }
      }
      {
        j = 0;
        while ((j < ((((b + 1)) * ((b + 1))) * 4)))
        {
          N[1][j] = N[0][N[0][j]];
          j += 1;
        }
      }
      memcpy(N[0], N[1], cpp_sizeof((N[0])));
      it += 1;
    }
  }
  {
    i = 0;
    while ((i < q))
    {
      printf("%d %d\n", (V[i] / ((4 * ((b + 1))))), (((V[i] / 4)) % ((b + 1))));
      i += 1;
    }
  }
  return 0;
}
