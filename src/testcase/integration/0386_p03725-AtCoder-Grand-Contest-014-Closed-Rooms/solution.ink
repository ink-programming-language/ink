// Translated from solution.cpp.

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var mp = cpp_expression("#include");

var a = cpp_array(810, 810);

var ans: dynamic;

var q: dynamic;

var dir = [0, 1, 1, 0, -1, 0, 0, -1];

var n: dynamic;

var m: dynamic;

var k: dynamic;

func IN(x: dynamic, y: dynamic)
{
  return ((((x >= 0) && (x < n)) && (y >= 0)) && (y < m));
}

func main()
{
  scanf("%d%d%d", (&n), (&m), (&k));
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", a[i]);
      {
        var j = 0;
        while ((j < m))
        {
          if ((a[i][j] == cpp_char("S")))
          {
            x = i;
            y = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  q.push(mp(mp(x, y), k));
  a[x][y] = cpp_char("#");
  var ans = 0x3f3f3f3f;
  while ((!q.empty()))
  {
    var t = q.front();
    q.pop();
    var x = t.fi.fi;
    var y = t.fi.se;
    var cnt = t.se;
    ans = min(ans, ((((min(min(x, y), min(((n - x) - 1), ((m - y) - 1))) + k) - 1)) / k));
    if ((t.se == 0))
    {
      continue;
    }
    {
      var i = 0;
      while ((i < 4))
      {
        var xx = (x + dir[i][0]);
        var yy = (y + dir[i][1]);
        if (((!IN(xx, yy)) || (a[xx][yy] != cpp_char("."))))
        {
          i += 1;
          continue;
        }
        a[xx][yy] = cpp_char("#");
        q.push(mp(mp(xx, yy), (cnt - 1)));
        i += 1;
      }
    }
  }
  printf("%d\n", (ans + 1));
  return 0;
}
