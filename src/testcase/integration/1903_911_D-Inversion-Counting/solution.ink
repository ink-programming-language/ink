// Translated from solution.cpp.

var MOD = (1e9 + 7);

var INF = 0x3f3f3f3f;

var LL_INF = 0x3f3f3f3f3f3f3f3f;

var PI = acos(-1);

var ERR = 1e-8;

var MAXN = 1e7;

var a = cpp_array(2000);

var n: dynamic;

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var l: dynamic;
  var r: dynamic;
  var m: dynamic;
  scanf("%d", (&m));
  var c = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = (i + 1);
        while ((j <= n))
        {
          if ((a[i] > a[j]))
          {
            c += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    scanf("%d%d", (&l), (&r));
    if ((l < r))
    {
      var t = (((r - l) + 1));
      c += ((((t * ((t - 1))) / 2)) % 2);
    }
    if ((!((c % 2))))
    {
      printf("even\n");
    } else
    {
      printf("odd\n");
    }
  }
  return 0;
}
