// Translated from solution.cpp.

var N = 1002;

var M = 123;

var Pi = acos(-1);

var Inf = 1e18;

var inf = 1e9;

var mod = (1e9 + 7);

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
}

func mult(a: dynamic, b: dynamic)
{
  return (((1 * a) * b) % mod);
}

var n: dynamic;

var m: dynamic;

var t = cpp_array(N, N, 2, 2);

func get(x: dynamic, y: dynamic)
{
  var ans = 0;
  {
    var i = x;
    while ((i > 0))
    {
      {
        var j = y;
        while ((j > 0))
        {
          ans ^= t[(x & 1)][(y & 1)][i][j];
          j = (((j & ((j + 1)))) - 1);
        }
      }
      i = (((i & ((i + 1)))) - 1);
    }
  }
  return ans;
}

func upd(x: dynamic, y: dynamic, v: dynamic)
{
  {
    var i = x;
    while ((i <= n))
    {
      {
        var j = y;
        while ((j <= n))
        {
          t[(x & 1)][(y & 1)][i][j] ^= v;
          j = ((j | ((j + 1))));
        }
      }
      i = ((i | ((i + 1))));
    }
  }
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var it = 0;
    while ((it < m))
    {
      var t: dynamic;
      var x1: dynamic;
      var y1: dynamic;
      var x2: dynamic;
      var y2: dynamic;
      var v: dynamic;
      scanf("%d%d%d%d%d", (&t), (&x1), (&y1), (&x2), (&y2));
      if ((t == 1))
      {
        var ans = (((get(x2, y2) ^ get((x1 - 1), (y1 - 1))) ^ get((x1 - 1), y2)) ^ get(x2, (y1 - 1)));
        printf("%d\n", ans);
      } else
      {
        scanf("%d", (&v));
        upd(x1, y1, v);
        upd((x2 + 1), y1, v);
        upd(x1, (y2 + 1), v);
        upd((x2 + 1), (y2 + 1), v);
      }
      it += 1;
    }
  }
  return 0;
}
