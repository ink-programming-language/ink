// Translated from solution.cpp.

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var mp: dynamic = cpp_expression("#include");

var a: dynamic = cpp_array(810, 810);

var ans: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var dir: dynamic = [0, 1, 1, 0, -1, 0, 0, -1];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func IN(x: dynamic, y: dynamic) -> dynamic
{
  return ((((x >= 0) && (x < n)) && (y >= 0)) && (y < m));
}

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&k));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%s", a[i]);
      {
        var j: dynamic = 0;
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
  var ans: dynamic = 0x3f3f3f3f;
  while ((!q.empty()))
  {
    var t: dynamic = q.front();
    q.pop();
    var x: dynamic = t.fi.fi;
    var y: dynamic = t.fi.se;
    var cnt: dynamic = t.se;
    ans = min(ans, ((((min(min(x, y), min(((n - x) - 1), ((m - y) - 1))) + k) - 1)) / k));
    if ((t.se == 0))
    {
      continue;
    }
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        var xx: dynamic = (x + dir[i][0]);
        var yy: dynamic = (y + dir[i][1]);
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
