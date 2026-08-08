// Translated from solution.cpp.

var M: dynamic;

var N: dynamic;

var K: dynamic;

var D: dynamic;

var cal = cpp_array(6);

var HE = cpp_expression("#in");

var CH = cpp_expression("#incl");

var pb = cpp_expression("#include");

var INF = cpp_expression("#includ");

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var c: dynamic;
  func Edge(u: dynamic, v: dynamic, c: dynamic)
  {
      this->u = cpp_construct(u);
      this->v = cpp_construct(v);
      this->c = cpp_construct(c);
    }
}

func toNum(s: dynamic)
{
  if ((s[0] == cpp_char("H")))
  {
    return HE;
  } else if ((s[0] == cpp_char("D")))
  {
    return CH;
  } else
  {
    var d = s;
    s.assign((s.begin() + 1), s.end());
    var t: dynamic;
    (ss >> t);
    if ((d[0] == cpp_char("C")))
    {
      return (t - 1);
    }
    return ((t + M) + 1);
  }
}

func main()
{
  while (cpp_comma(((((cin >> M) >> N) >> K) >> D), ((((M || N) || K) || D))))
  {
    var eg: dynamic;
    {
      var i = 0;
      while ((i < M))
      {
        read(cal[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < D))
      {
        var a: dynamic;
        var b: dynamic;
        var s: dynamic;
        var d: dynamic;
        var c: dynamic;
        read(a, b, c);
        s = toNum(a);
        d = toNum(b);
        eg.pb(Edge(s, d, (c * K)));
        eg.pb(Edge(d, s, (c * K)));
        i += 1;
      }
    }
    var dist = cpp_array(((1 << 6)), 300);
    {
      var i = 0;
      while ((i < ((1 << M))))
      {
        {
          var j = 0;
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
      var update = false;
      {
        var i = 0;
        while ((i < eg.size()))
        {
          {
            var j = 0;
            while ((j < ((1 << M))))
            {
              var e = eg[i];
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
    var ans = INF;
    {
      var i = 0;
      while ((i < ((1 << M))))
      {
        ans = min(ans, dist[CH][i]);
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
