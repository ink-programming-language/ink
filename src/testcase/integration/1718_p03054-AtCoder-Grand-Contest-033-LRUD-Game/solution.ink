// Translated from solution.cpp.

var h: dynamic;

var w: dynamic;

var n: dynamic;

var sr: dynamic;

var sc: dynamic;

var t = cpp_array(200001);

var a = cpp_array(200001);

var d = [[0, 1], [0, -1], [1, 0], [-1, 0]];

var dir = "RLDU";

func main()
{
  scanf("%d %d %d", (&h), (&w), (&n));
  scanf("%d %d", (&sr), (&sc));
  scanf("%s", t);
  scanf("%s", a);
  var by: dynamic;
  var bx: dynamic;
  var flag = 0;
  {
    var i = 0;
    while ((i < 4))
    {
      by = sr;
      bx = sc;
      var temp = ((i - (((i % 2)) * 2)) + 1);
      {
        var j = 0;
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
