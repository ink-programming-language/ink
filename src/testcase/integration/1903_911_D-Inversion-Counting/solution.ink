// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

var INF: dynamic = 0x3f3f3f3f;

var LL_INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var PI: dynamic = acos(-1);

var ERR: dynamic = 1e-8;

var MAXN: dynamic = 1e7;

var a: dynamic = cpp_array(2000);

var n: dynamic = cpp_uninitialized();

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d", (&m));
  var c: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + 1);
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
      var t: dynamic = (((r - l) + 1));
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
