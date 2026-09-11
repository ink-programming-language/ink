// Translated from solution.cpp.

var M: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var cal: dynamic = cpp_array(6);

var HE: dynamic = cpp_expression("#in");

var CH: dynamic = cpp_expression("#incl");

var pb: dynamic = cpp_expression("#include");

var INF: dynamic = cpp_expression("#includ");

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  func Edge(u: dynamic, v: dynamic, c: dynamic) -> dynamic
  {
      self->u = cpp_construct(u);
      self->v = cpp_construct(v);
      self->c = cpp_construct(c);
    }
}

func toNum(s: dynamic) -> dynamic
{
  if ((s[0] == cpp_char("H")))
  {
    return HE;
  } else if ((s[0] == cpp_char("D")))
  {
    return CH;
  } else
  {
    var d: dynamic = s;
    s.assign((s.begin() + 1), s.end());
    var t: dynamic = cpp_uninitialized();
    (ss >> t);
    if ((d[0] == cpp_char("C")))
    {
      return (t - 1);
    }
    return ((t + M) + 1);
  }
}

func main() -> dynamic
{
  while (cpp_comma(((((cin >> M) >> N) >> K) >> D), ((((M || N) || K) || D))))
  {
    var eg: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < M))
      {
        read(cal[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < D))
      {
        var a: dynamic = cpp_uninitialized();
        var b: dynamic = cpp_uninitialized();
        var s: dynamic = cpp_uninitialized();
        var d: dynamic = cpp_uninitialized();
        var c: dynamic = cpp_uninitialized();
        read(a, b, c);
        s = toNum(a);
        d = toNum(b);
        eg.pb(Edge(s, d, (c * K)));
        eg.pb(Edge(d, s, (c * K)));
        i += 1;
      }
    }
    var dist: dynamic = cpp_array(((1 << 6)), 300);
    {
      var i: dynamic = 0;
      while ((i < ((1 << M))))
      {
        {
          var j: dynamic = 0;
          while ((j < ((M + N) + 2)))
          {
            dist[j][i] = INF;
            j += 1;
          }
        }
        i += 1;
      }
    }
    dist[HE][0] = 0;
    while (1)
    {
      var update: dynamic = false;
      {
        var i: dynamic = 0;
        while ((i < eg.size()))
        {
          {
            var j: dynamic = 0;
            while ((j < ((1 << M))))
            {
              var e: dynamic = eg[i];
              if ((dist[e.u][j] == INF))
              {
                j += 1;
                continue;
              }
              if ((e.v < M))
              {
                if ((j & ((1 << e.v))))
                {
                  j += 1;
                  continue;
                }
                if ((((dist[e.u][j] + e.c) - cal[e.v]) < dist[e.v][(j | ((1 << e.v)))]))
                {
                  update = true;
                  dist[e.v][(j | ((1 << e.v)))] = ((dist[e.u][j] + e.c) - cal[e.v]);
                }
              } else
              {
                if (((dist[e.u][j] + e.c) < dist[e.v][j]))
                {
                  update = true;
                  dist[e.v][j] = (dist[e.u][j] + e.c);
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      if ((!update))
      {
        break;
      }
    }
    var ans: dynamic = INF;
    {
      var i: dynamic = 0;
      while ((i < ((1 << M))))
      {
        ans = min(ans, dist[CH][i]);
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
