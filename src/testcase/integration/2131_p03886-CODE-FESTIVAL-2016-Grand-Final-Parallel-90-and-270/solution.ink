// Translated from solution.cpp.

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

func MoveX(p: dynamic, x: dynamic) -> dynamic
{
  for (var q: dynamic in p)
  {
    if ((q.x >= x))
    {
      q.x += 1;
    }
  }
}

func MoveY(p: dynamic, y: dynamic) -> dynamic
{
  for (var q: dynamic in p)
  {
    if ((q.y >= y))
    {
      q.y += 1;
    }
  }
}

func Signum(x: dynamic) -> dynamic
{
  return ( ((x > 0)) ? 1 : ( ((x < 0)) ? -1 : 0));
}

func Solve(a: dynamic) -> dynamic
{
  if ((a.size() == 4))
  {
    return [[0, 0], [1, 0], [1, 1], [0, 1]];
  }
  var n: dynamic = cpp_cast(a.size());
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      if (((a[i] + a[(i + 1)]) == 360))
      {
        var b: dynamic = a;
        b.erase((b.begin() + i), ((b.begin() + i) + 2));
        var p: dynamic = Solve(b);
        var pr: dynamic = ( ((i == 0)) ? (n - 3) : (i - 1));
        var ne: dynamic = ( ((i == (n - 2))) ? 0 : i);
        MoveX(p, (p[pr].x + 1));
        MoveX(p, p[pr].x);
        MoveY(p, (p[pr].y + 1));
        MoveY(p, p[pr].y);
        var u: dynamic = p[pr];
        var v: dynamic = p[ne];
        if ((u.x == v.x))
        {
          var dy: dynamic = ( ((u.y > v.y)) ? -1 : 1);
          var dx: dynamic = Signum((p[((((pr + n) - 3)) % ((n - 2)))].x - u.x));
          if ((b[pr] != a[i]))
          {
            dx *= -1;
          }
          p[ne].x += dx;
          p.insert((p.begin() + i), [u.x, (u.y + dy)]);
          p.insert(((p.begin() + i) + 1), [(u.x + dx), (u.y + dy)]);
        } else
        {
          var dx: dynamic = ( ((u.x > v.x)) ? -1 : 1);
          var dy: dynamic = Signum((p[((((pr + n) - 3)) % ((n - 2)))].y - u.y));
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var res: dynamic = Solve(a);
  for (var p: dynamic in res)
  {
    write(p.x, " ", p.y, cpp_char("\n"));
  }
  return 0;
}
