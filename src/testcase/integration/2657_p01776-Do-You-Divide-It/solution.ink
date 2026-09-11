// Translated from solution.cpp.

func EQ(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);i++)");
}

var fs: dynamic = cpp_expression("#incl");

var sc: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include<");

var sz: dynamic = cpp_expression("#inclu");

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var EPS: dynamic = 1e-8;

var PI: dynamic = acos(-1);

var MAX_X: dynamic = 1000;

var MAX_Y: dynamic = 1000;

func unit(p: dynamic) -> dynamic
{
  return (p / abs(p));
}

func norm(p: dynamic) -> dynamic
{
  return make_pair((p * P(0, 1)), (p * P(0, -1)));
}

func dot(x: dynamic, y: dynamic) -> dynamic
{
  return real((conj(x) * y));
}

func cross(x: dynamic, y: dynamic) -> dynamic
{
  return imag((conj(x) * y));
}

func para(a: dynamic, b: dynamic) -> dynamic
{
  return (abs(cross((a.fs - a.sc), (b.fs - b.sc))) < EPS);
}

func ccw(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  b -= a;
  c -= a;
  if ((cross(b, c) > EPS))
  {
    return 1;
  }
  if ((cross(b, c) < (-EPS)))
  {
    return -1;
  }
  if ((dot(b, c) < (-EPS)))
  {
    return 2;
  }
  if (((abs(b) + EPS) < abs(c)))
  {
    return -2;
  }
  return 0;
}

func is_cp(a: dynamic, b: dynamic) -> dynamic
{
  if (((ccw(a.fs, a.sc, b.fs) * ccw(a.fs, a.sc, b.sc)) <= 0))
  {
    if (((ccw(b.fs, b.sc, a.fs) * ccw(b.fs, b.sc, a.sc)) <= 0))
    {
      return true;
    }
  }
  return false;
}

func line_cp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.fs + ((((a.sc - a.fs)) * cross((b.sc - b.fs), (b.fs - a.fs))) / cross((b.sc - b.fs), (a.sc - a.fs))));
}

func area(p: dynamic) -> dynamic
{
  if ((p.sz < 3))
  {
    return 0;
  }
  var res: dynamic = cross(p[(p.sz - 1)], p[0]);
  rep(i, (p.sz - 1)) += cross(p[i], p[(i + 1)]);
  return (res / 2);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var S: dynamic = area(p);
  var v: dynamic = cpp_construct(((4 * MAX_X) + 1));
  {
    var i: dynamic = (-2 * MAX_X);
    while ((i <= (2 * MAX_X)))
    {
      var l: dynamic = L(P(i, ((-2 * MAX_Y) - 1)), P(i, ((2 * MAX_Y) + 1)));
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var seg: dynamic = L(p[j], p[(((j + 1)) % n)]);
          if (((!para(seg, l)) && is_cp(seg, l)))
          {
            var cp: dynamic = line_cp(seg, l);
            if ((abs((cp - ( ((seg.fs.real() < seg.sc.real())) ? seg.sc : seg.fs))) > EPS))
            {
              v[(i + (2 * MAX_X))].push_back(cp.imag());
            }
          }
          j += 1;
        }
      }
      sort(v[(i + (2 * MAX_X))].begin(), v[(i + (2 * MAX_X))].end());
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (2 * MAX_X)))
    {
      assert((((v[(2 * i)].size() % 2) == 0) && ((v[((2 * i) + 1)].size() % 2) == 0)));
      assert((v[(2 * i)].size() == v[((2 * i) + 1)].size()));
      {
        var j: dynamic = 0;
        while ((j < v[(2 * i)].size()))
        {
          sum += (((abs((v[(2 * i)][j] - v[(2 * i)][(j + 1)])) + abs((v[((2 * i) + 1)][j] - v[((2 * i) + 1)][(j + 1)])))) / 2);
          j += 2;
        }
      }
      i += 1;
    }
  }
  write(fixed, setprecision(9), (max(sum, (S - sum)) / 4), "\n");
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(x, y);
    x *= 2;
    y *= 2;
    p[i] = P(x, y);
  }
