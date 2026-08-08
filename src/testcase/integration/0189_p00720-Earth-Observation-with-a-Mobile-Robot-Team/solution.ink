// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func sq(x: dynamic)
{
  return cpp_expression("#include<");
}

func sqsum(x: dynamic, y: dynamic)
{
  return cpp_expression("#include<cmat");
}

class path
{
  var n: dynamic;
  var t: dynamic = cpp_array(1001);
  var x: dynamic = cpp_array(1001);
  var y: dynamic = cpp_array(1001);
}

class event
{
  var t: dynamic;
  var u: dynamic;
  var v: dynamic;
  var io: dynamic;
  func operator_less(e: dynamic)
  {
      return (t < e.t);
    }
}

func solve_quadratic_equation(a: dynamic, b: dynamic, c: dynamic)
{
  if ((a == 0))
  {
    return vector(0);
  }
  if ((((b * b) - ((4 * a) * c)) < 0))
  {
    return vector(0);
  }
  var res = cpp_construct(2);
  res[0] = ((((-b) - sqrt(((b * b) - ((4 * a) * c))))) / ((2 * a)));
  res[1] = ((((-b) + sqrt(((b * b) - ((4 * a) * c))))) / ((2 * a)));
  return res;
}

func calc_point(P: dynamic, t: dynamic, x: dynamic, y: dynamic)
{
  x = P.x[0];
  y = P.y[0];
  rep(i, (P.n - 1));
  {
    if ((t <= P.t[(i + 1)]))
    {
      x += (((t - P.t[i])) * P.x[(i + 1)]);
      y += (((t - P.t[i])) * P.y[(i + 1)]);
      return;
    }
    x += (((P.t[(i + 1)] - P.t[i])) * P.x[(i + 1)]);
    y += (((P.t[(i + 1)] - P.t[i])) * P.y[(i + 1)]);
  }
  assert(0);
}

var R: dynamic;

func calc_touch_points(P: dynamic, Q: dynamic, u: dynamic, v: dynamic, E: dynamic)
{
  var t: dynamic;
  t.insert(t.end(), P.t, (P.t + P.n));
  t.insert(t.end(), Q.t, (Q.t + Q.n));
  sort(t.begin(), t.end());
  t.erase(unique(t.begin(), t.end()), t.end());
  rep(k, (t.size() - 1));
  {
    var x1: dynamic;
    var y1: dynamic;
    var x2: dynamic;
    var y2: dynamic;
    var x3: dynamic;
    var y3: dynamic;
    var x4: dynamic;
    var y4: dynamic;
    calc_point(P, t[k], x1, y1);
    calc_point(P, t[(k + 1)], x2, y2);
    calc_point(Q, t[k], x3, y3);
    calc_point(Q, t[(k + 1)], x4, y4);
    var a = sqsum((((x2 - x1) + x3) - x4), (((y2 - y1) + y3) - y4));
    var b = (((2 * ((((x2 - x1) + x3) - x4))) * ((x1 - x3))) + ((2 * ((((y2 - y1) + y3) - y4))) * ((y1 - y3))));
    var c = (sqsum((x1 - x3), (y1 - y3)) - (R * R));
    var sol = solve_quadratic_equation(a, b, c);
    if ((sol.size() == 2))
    {
      if (((0 <= sol[0]) && (sol[0] <= 1)))
      {
        E.push_back([(t[k] + (sol[0] * ((t[(k + 1)] - t[k])))), u, v, 0]);
      }
      if (((0 <= sol[1]) && (sol[1] <= 1)))
      {
        E.push_back([(t[k] + (sol[1] * ((t[(k + 1)] - t[k])))), u, v, 1]);
      }
    }
  }
}

func main()
{
  {
    var n: dynamic;
    var T: dynamic;
    while (cpp_comma(scanf("%d%d%d", (&n), (&T), (&R)), n))
    {
      var s = cpp_array(9, 100);
      var P = cpp_array(100);
      var E: dynamic;
      rep(j, n);
      rep(i, j);
      calc_touch_points(P[i], P[j], i, j, E);
      sort(E.begin(), E.end());
      var ans = [true];
      var adj = [];
      rep(i, n);
      rep(j, n);
      if (((i != j) && (sqsum((P[i].x[0] - P[j].x[0]), (P[i].y[0] - P[j].y[0])) <= (R * R))))
      {
        adj[i][j] = cpp_assign(adj[j][i], "=", true);
      }
      rep(cpp_name, n);
      rep(i, n);
      rep(j, n);
      if (adj[i][j])
      {
        ans[i] |= ans[j];
        ans[j] |= ans[i];
      }
      var Q: dynamic;
      rep(i, E.size());
      {
        var e = E[i];
        var u = e.u;
        var v = e.v;
        if ((e.io == 0))
        {
          adj[u][v] = cpp_assign(adj[v][u], "=", true);
          if ((ans[u] == ans[v]))
          {
            continue;
          }
          var tar = (if (ans[u]) v else u);
          ans[tar] = true;
          assert(Q.empty());
          Q.push(tar);
          while ((!Q.empty()))
          {
            var w = Q.front();
            Q.pop();
            rep(x, n);
            if ((adj[w][x] && (!ans[x])))
            {
              ans[x] = true;
              Q.push(x);
            }
          }
        } else
        {
          adj[u][v] = cpp_assign(adj[v][u], "=", false);
        }
      }
      var s_ans: dynamic;
      rep(i, n);
      if (ans[i])
      {
        s_ans.push_back(s[i]);
      }
      sort(s_ans.begin(), s_ans.end());
      rep(i, s_ans.size());
      puts(s_ans[i].c_str());
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        scanf("%s", s[i]);
        var m = 0;
        var t: dynamic;
        var x: dynamic;
        var y: dynamic;
        while (1)
        {
          scanf("%d%d%d", (&t), (&x), (&y));
          P[i].t[m] = t;
          P[i].x[m] = x;
          P[i].y[m] = y;
          m += 1;
          if ((t == T))
          {
            break;
          }
        }
        P[i].n = m;
      }
