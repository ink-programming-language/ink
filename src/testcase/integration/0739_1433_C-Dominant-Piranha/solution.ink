// Translated from solution.cpp.

var mod: dynamic = (1e8 + 7);

var N: dynamic = (3e5 + 10);

var vis: dynamic = cpp_array(N);

func main() -> dynamic
{
  var num: dynamic = cpp_uninitialized();
  scanf("%d", (&num));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while (cpp_update(num, "--"))
  {
    scanf("%d", (&n));
    var maxx: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&vis[i]));
        maxx = max(vis[i], maxx);
        i += 1;
      }
    }
    var num: dynamic = -1;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((vis[i] == maxx))
        {
          if ((((i - 1) >= 0) && (vis[(i - 1)] < maxx)))
          {
            num = (i + 1);
            break;
          }
          if ((((i + 1) < n) && (vis[(i + 1)] < maxx)))
          {
            num = (i + 1);
            break;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", num);
  }
  return 0;
}
