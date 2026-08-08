// Translated from solution.cpp.

var WMAX = cpp_expression("#in");

var HMAX = cpp_expression("#inc");

var INF = cpp_expression("#includ");

var M = cpp_array(HMAX);

var h: dynamic;

var ha: dynamic;

var hb: dynamic;

var wa: dynamic;

var wb: dynamic;

var visited = cpp_array(WMAX, HMAX);

var flag45_1: dynamic;

var flag45_2: dynamic;

class A
{
  var S: dynamic;
  var N: dynamic;
  func A(S: dynamic, N: dynamic)
  {
      this->S = S;
      this->N = N;
    }
  func operator_less(a: dynamic)
  {
      return (S < a.S);
    }
}

func rec(x: dynamic, y: dynamic, bi: dynamic)
{
  var dx = [1, 1, 0, -1, -1, -1, 0, 1];
  var dy = [0, 1, 1, 1, 0, -1, -1, -1];
  ha = min(ha, y);
  hb = max(hb, y);
  wa = min(wa, x);
  wb = max(wb, x);
  {
    var i = 0;
    while ((i < 8))
    {
      var nx = (x + dx[i]);
      var ny = (y + dy[i]);
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

func solve()
{
  var cnt = cpp_array((WMAX * HMAX));
  var smax: dynamic;
  var smin: dynamic;
  var V: dynamic;
  smax = (-INF);
  smin = INF;
  {
    var i = 0;
    while ((i < (WMAX * HMAX)))
    {
      cnt[i] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < HMAX))
    {
      {
        var j = 0;
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
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < M[i].length()))
        {
          if (((!visited[i][j]) && (M[i][j] == cpp_char("*"))))
          {
            ha = cpp_assign(wa, "=", INF);
            hb = cpp_assign(wb, "=", (-INF));
            flag45_1 = cpp_assign(flag45_2, "=", 0);
            visited[i][j] = true;
            rec(j, i, -1);
            var nh = ((hb - ha) + 1);
            var nw = ((wb - wa) + 1);
            var S: dynamic;
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
    var i = smin;
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
    var i = 0;
    while ((i < V.size()))
    {
      write(V[i].S, " ", V[i].N, "\n");
      i += 1;
    }
  }
}

func main()
{
  var first = true;
  while ((((cin >> h)) && (h != 0)))
  {
    getchar();
    {
      var i = 0;
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
