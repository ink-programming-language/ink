// Translated from solution.cpp.

var EPS = 1e-8;

func dot(a: dynamic, b: dynamic)
{
  return real((conj(a) * b));
}

func cross(a: dynamic, b: dynamic)
{
  return imag((conj(a) * b));
}

func ccw(a: dynamic, b: dynamic, c: dynamic)
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
  if ((norm(b) < norm(c)))
  {
    return -2;
  }
  return 0;
}

func crossPoint(l: dynamic, m: dynamic)
{
  var A = cross((l.second - l.first), (m.second - m.first));
  var B = cross((l.second - l.first), (l.second - m.first));
  if (((fabs(A) < EPS) && (fabs(B) < EPS)))
  {
    return m.first;
  } else if ((fabs(A) >= EPS))
  {
    return (m.first + ((B / A) * ((m.second - m.first))));
  }
}

func convex_cut(G: dynamic, l: dynamic)
{
  var res: dynamic;
  {
    var i = 0;
    while ((i < G.size()))
    {
      var A = G[i];
      var B = G[(((i + 1)) % G.size())];
      if ((ccw(l.first, l.second, A) != -1))
      {
        res.push_back(A);
      }
      if (((ccw(l.first, l.second, A) * ccw(l.first, l.second, B)) < 0))
      {
        res.push_back(crossPoint(L(A, B), l));
      }
      i += 1;
    }
  }
  return res;
}

func check(G: dynamic, d: dynamic)
{
  var pol = G;
  var n = G.size();
  {
    var i = 0;
    while ((i < n))
    {
      var c = ((G[(((i + 1)) % n)] - G[i]));
      var a = ((P(abs(c), d) * ((c / abs(c)))) + G[i]);
      pol = convex_cut(pol, L(a, (a + c)));
      if ((pol.size() < 3))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    {
      var i = 0;
      var x: dynamic;
      var y: dynamic;
      while ((i < n))
      {
        read(x, y);
        pol[i] = P(x, y);
        i += 1;
      }
    }
    var L = 0;
    var M: dynamic;
    var R = 1e4;
    while ((L < R))
    {
      M = (((L + R)) / 2);
      if ((!check(pol, M)))
      {
        R = (M - EPS);
      } else
      {
        L = (M + EPS);
      }
    }
    printf("%lf\n", L);
  }
  return 0;
}
