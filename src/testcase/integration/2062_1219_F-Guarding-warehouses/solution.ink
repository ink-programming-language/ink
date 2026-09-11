// Translated from solution.cpp.

var PI: dynamic = acosl(-1);

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

class point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func point() -> dynamic
  {
    }
  func point(x: dynamic, y: dynamic) -> dynamic
  {
      self->x = cpp_construct(x);
      self->y = cpp_construct(y);
    }
  func kampas() -> dynamic
  {
      return atan2l(y, x);
    }
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return point((a.x + b.x), (a.y + b.y));
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return point((a.x - b.x), (a.y - b.y));
}

func operator_multiply(a: dynamic, k: dynamic) -> dynamic
{
  return point((a.x * k), (a.y * k));
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x * b.y) - (a.y * b.x));
}

func f(alfa: dynamic, beta: dynamic, ab: dynamic) -> dynamic
{
  var a: dynamic = ab.first;
  var b: dynamic = ab.second;
  va = (va * ((cross(a, b) / cross(va, (b - a)))));
  vb = (vb * ((cross(a, b) / cross(vb, (b - a)))));
  return (cross(va, vb) / 2);
}

func calc(alfa: dynamic, beta: dynamic, A: dynamic, mn: dynamic = [point(1e10, 1e10), point(1e10, 1e10)], mn1: dynamic = [point(1e10, 1e10), point(1e10, 1e10)]) -> dynamic
{
  if (A.empty())
  {
    if ((mn1.first.x < 1e9))
    {
      return (f(alfa, beta, mn1) - f(alfa, beta, mn));
    } else
    {
      return 0;
    }
  }
  if (((beta - alfa) < 1e-9))
  {
    return 0;
  }
  var L: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  var gamma: dynamic = (((alfa + beta)) / 2);
  var ok: dynamic = true;
  for (var i: dynamic in A)
  {
    var k1: dynamic = i.first.kampas();
    var k2: dynamic = i.second.kampas();
    if (((beta < k1) || (k2 < alfa)))
    {
      continue;
    }
    if (((beta - alfa) < 0.2))
    {
      if (((k1 < (alfa + 1e-9)) && (beta < (k2 + 1e-9))))
      {
        var k1: dynamic = abs((cross(mn.first, mn.second) / cross(v, (mn.second - mn.first))));
        var k2: dynamic = abs((cross(mn1.first, mn1.second) / cross(v, (mn1.second - mn1.first))));
        var d: dynamic = abs((cross(i.first, i.second) / cross(v, (i.second - i.first))));
        if (((mn.first.x >= 1e9) || (d <= k1)))
        {
          mn1 = mn;
          mn = i;
        } else if (((mn1.first.x >= 1e9) || (d <= k2)))
        {
          mn1 = i;
        }
        continue;
      }
    }
    ok = false;
    if ((k1 < gamma))
    {
      L.push_back(i);
    }
    if ((k2 > gamma))
    {
      R.push_back(i);
    }
  }
  if (ok)
  {
    return (f(alfa, beta, mn1) - f(alfa, beta, mn));
  }
  return (calc(alfa, gamma, L, mn, mn1) + calc(gamma, beta, R, mn, mn1));
}

func sgn(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    return -1;
  }
  if ((x > 0))
  {
    return 1;
  }
  return 0;
}

func main() -> dynamic
{
  write(fixed, setprecision(3));
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var A: dynamic = cpp_uninitialized();
  while (cpp_update(n, "--"))
  {
    var k: dynamic = cpp_uninitialized();
    read(k);
    var a0: dynamic = cpp_uninitialized();
    read(a0.x, a0.y);
    var aj: dynamic = a0;
    {
      var i: dynamic = 1;
      while ((i < k))
      {
        var ai: dynamic = cpp_uninitialized();
        read(ai.x, ai.y);
        A.push_back([aj, ai]);
        aj = ai;
        i += 1;
      }
    }
    A.push_back([aj, a0]);
  }
  n = A.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (((abs(A[i].first.y) < 0.5) || (abs(A[i].second.y) < 0.5)))
      {
        i += 1;
        continue;
      }
      if ((((A[i].first.y > 0)) != ((A[i].second.y > 0))))
      {
        var k: dynamic = ((-A[i].first.y) / ((A[i].second - A[i].first)).y);
        var B: dynamic = (A[i].first + (((A[i].second - A[i].first)) * k));
        A.push_back([B, A[i].second]);
        A[i].second = B;
      }
      i += 1;
    }
  }
  var A: dynamic = cpp_uninitialized();
  for (var b: dynamic in A)
  {
    if ((abs((b.first.kampas() - b.second.kampas())) > 1e-8))
    {
      A.push_back(b);
    }
  }
  for (var i: dynamic in A)
  {
    while ((sgn(i.first.y) != sgn(i.second.y)))
    {
      if ((abs(i.first.y) < abs(i.second.y)))
      {
        if ((i.second.y > 0))
        {
          i.first.y = 1e-11;
        } else
        {
          i.first.y = -1e-11;
        }
      } else
      {
        if ((i.first.y > 0))
        {
          i.second.y = 1e-11;
        } else
        {
          i.second.y = -1e-11;
        }
      }
    }
    if ((i.first.kampas() > i.second.kampas()))
    {
      swap(i.first, i.second);
    }
  }
  write(fixed, setprecision(20), calc(((-PI) - 0.001), (PI + 0.001), A), "\n");
}
