// Translated from solution.cpp.

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var sr: dynamic = cpp_uninitialized();

var sc: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(200001);

var a: dynamic = cpp_array(200001);

var d: dynamic = [[0, 1], [0, -1], [1, 0], [-1, 0]];

var dir: dynamic = "RLDU";

func main() -> dynamic
{
  scanf("%d %d %d", (&h), (&w), (&n));
  scanf("%d %d", (&sr), (&sc));
  scanf("%s", t);
  scanf("%s", a);
  var by: dynamic = cpp_uninitialized();
  var bx: dynamic = cpp_uninitialized();
  var flag: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      by = sr;
      bx = sc;
      var temp: dynamic = ((i - (((i % 2)) * 2)) + 1);
      {
        var j: dynamic = 0;
        while (((j < n) && (flag == 0)))
        {
          if ((t[j] == dir[i]))
          {
            by += d[i][0];
            bx += d[i][1];
          }
          if (((((by > h) || (by < 1)) || (bx > w)) || (bx < 1)))
          {
            flag = 1;
            break;
          }
          if ((a[j] == dir[temp]))
          {
            if ((((((by + d[temp][0]) > h) || ((by + d[temp][0]) < 1)) || ((bx + d[temp][1]) > w)) || ((bx + d[temp][1]) < 1)))
            {
              j += 1;
              continue;
            }
            by += d[temp][0];
            bx += d[temp][1];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((flag == 0))
  {
    printf("YES");
  } else
  {
    printf("NO");
  }
  return 0;
}
