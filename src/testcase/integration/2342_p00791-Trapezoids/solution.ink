// Translated from solution.cpp.

var WMAX: dynamic = cpp_expression("#in");

var HMAX: dynamic = cpp_expression("#inc");

var INF: dynamic = cpp_expression("#includ");

var M: dynamic = cpp_array(HMAX);

var h: dynamic = cpp_uninitialized();

var ha: dynamic = cpp_uninitialized();

var hb: dynamic = cpp_uninitialized();

var wa: dynamic = cpp_uninitialized();

var wb: dynamic = cpp_uninitialized();

var visited: dynamic = cpp_array(WMAX, HMAX);

var flag45_1: dynamic = cpp_uninitialized();

var flag45_2: dynamic = cpp_uninitialized();

class A
{
  var S: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  func A(S: dynamic, N: dynamic) -> dynamic
  {
      self->S = S;
      self->N = N;
    }
  func operator_less(a: dynamic) -> dynamic
  {
      return (S < a.S);
    }
}

func rec(x: dynamic, y: dynamic, bi: dynamic) -> dynamic
{
  var dx: dynamic = [1, 1, 0, -1, -1, -1, 0, 1];
  var dy: dynamic = [0, 1, 1, 1, 0, -1, -1, -1];
  ha = min(ha, y);
  hb = max(hb, y);
  wa = min(wa, x);
  wb = max(wb, x);
  {
    var i: dynamic = 0;
    while ((i < 8))
    {
      var nx: dynamic = (x + dx[i]);
      var ny: dynamic = (y + dy[i]);
      if (((ny < 0) || (ny >= h)))
      {
        i += 1;
        continue;
      }
      if (((nx < 0) || (nx >= M[ny].length())))
      {
        i += 1;
        continue;
      }
      if (visited[ny][nx])
      {
        i += 1;
        continue;
      }
      if ((M[ny][nx] != cpp_char("*")))
      {
        i += 1;
        continue;
      }
      visited[ny][nx] = true;
      if ((i != bi))
      {
        if (((i == 1) || (i == 5)))
        {
          flag45_1 += 1;
        }
        if (((i == 3) || (i == 7)))
        {
          flag45_2 += 1;
        }
      }
      rec(nx, ny, i);
      i += 1;
    }
  }
}

func solve() -> dynamic
{
  var cnt: dynamic = cpp_array((WMAX * HMAX));
  var smax: dynamic = cpp_uninitialized();
  var smin: dynamic = cpp_uninitialized();
  var V: dynamic = cpp_uninitialized();
  smax = (-INF);
  smin = INF;
  {
    var i: dynamic = 0;
    while ((i < (WMAX * HMAX)))
    {
      cnt[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < HMAX))
    {
      {
        var j: dynamic = 0;
        while ((j < WMAX))
        {
          visited[i][j] = false;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < M[i].length()))
        {
          if (((!visited[i][j]) && (M[i][j] == cpp_char("*"))))
          {
            ha = cpp_assign(wa, "=", INF);
            hb = cpp_assign(wb, "=", (-INF));
            flag45_1 = cpp_assign(flag45_2, "=", 0);
            visited[i][j] = true;
            rec(j, i, -1);
            var nh: dynamic = ((hb - ha) + 1);
            var nw: dynamic = ((wb - wa) + 1);
            var S: dynamic = cpp_uninitialized();
            S = (nh * nw);
            if (((!flag45_1) && (!flag45_2)))
            {
            } else if (((flag45_1 == 2) || (flag45_2 == 2)))
            {
              S -= (((nh - 1)) * nh);
            } else if ((flag45_1 && flag45_2))
            {
              S -= (((nh - 1)) * nh);
            } else if (((flag45_1 + flag45_2) == 1))
            {
              S -= ((((nh - 1)) * nh) / 2);
            }
            cnt[S] += 1;
            smax = max(smax, S);
            smin = min(smin, S);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = smin;
    while ((i <= smax))
    {
      if (cnt[i])
      {
        V.push_back(A(i, cnt[i]));
      }
      i += 1;
    }
  }
  sort(V.begin(), V.end());
  {
    var i: dynamic = 0;
    while ((i < V.size()))
    {
      write(V[i].S, " ", V[i].N, "\n");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var first: dynamic = true;
  while ((((cin >> h)) && (h != 0)))
  {
    getchar();
    {
      var i: dynamic = 0;
      while ((i < h))
      {
        getline(cin, M[i]);
        i += 1;
      }
    }
    if (first)
    {
      first = false;
    } else
    {
      write("----------", "\n");
    }
    solve();
  }
}
