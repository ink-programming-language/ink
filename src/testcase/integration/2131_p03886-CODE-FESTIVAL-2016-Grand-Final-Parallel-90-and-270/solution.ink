// Translated from solution.cpp.

class Point
{
  var x: dynamic;
  var y: dynamic;
}

func MoveX(p: dynamic, x: dynamic)
{
  for (var q in p)
  {
    if ((q.x >= x))
    {
      q.x += 1;
    }
  }
}

func MoveY(p: dynamic, y: dynamic)
{
  for (var q in p)
  {
    if ((q.y >= y))
    {
      q.y += 1;
    }
  }
}

func Signum(x: dynamic)
{
  return (if ((x > 0)) 1 else (if ((x < 0)) -1 else 0));
}

func Solve(a: dynamic)
{
  if ((a.size() == 4))
  {
    return [[0, 0], [1, 0], [1, 1], [0, 1]];
  }
  var n = cpp_cast(a.size());
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      if (((a[i] + a[(i + 1)]) == 360))
      {
        var b = a;
        b.erase((b.begin() + i), ((b.begin() + i) + 2));
        var p = Solve(b);
        var pr = (if ((i == 0)) (n - 3) else (i - 1));
        var ne = (if ((i == (n - 2))) 0 else i);
        MoveX(p, (p[pr].x + 1));
        MoveX(p, p[pr].x);
        MoveY(p, (p[pr].y + 1));
        MoveY(p, p[pr].y);
        var u = p[pr];
        var v = p[ne];
        if ((u.x == v.x))
        {
          var dy = (if ((u.y > v.y)) -1 else 1);
          var dx = Signum((p[((((pr + n) - 3)) % ((n - 2)))].x - u.x));
          if ((b[pr] != a[i]))
          {
            dx *= -1;
          }
          p[ne].x += dx;
          p.insert((p.begin() + i), [u.x, (u.y + dy)]);
          p.insert(((p.begin() + i) + 1), [(u.x + dx), (u.y + dy)]);
        } else
        {
          var dx = (if ((u.x > v.x)) -1 else 1);
          var dy = Signum((p[((((pr + n) - 3)) % ((n - 2)))].y - u.y));
          if ((b[pr] != a[i]))
          {
            dy *= -1;
          }
          p[ne].y += dy;
          p.insert((p.begin() + i), [(u.x + dx), u.y]);
          p.insert(((p.begin() + i) + 1), [(u.x + dx), (u.y + dy)]);
        }
        return p;
      }
      i += 1;
    }
  }
  assert(false);
  return [];
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      sum += a[i];
      i += 1;
    }
  }
  if ((((n % 2) == 1) || (sum != (180 * ((n - 2))))))
  {
    write(-1, cpp_char("\n"));
    return 0;
  }
  var res = Solve(a);
  for (var p in res)
  {
    write(p.x, " ", p.y, cpp_char("\n"));
  }
  return 0;
}
