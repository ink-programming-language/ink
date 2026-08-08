// Translated from solution.cpp.

var mod = (1e8 + 7);

var N = (3e5 + 10);

var vis = cpp_array(N);

func main()
{
  var num: dynamic;
  scanf("%d", (&num));
  var n: dynamic;
  var m: dynamic;
  while (cpp_update(num, "--"))
  {
    scanf("%d", (&n));
    var maxx = 0;
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d", (&vis[i]));
        maxx = max(vis[i], maxx);
        i += 1;
      }
    }
    var num = -1;
    {
      var i = 0;
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
