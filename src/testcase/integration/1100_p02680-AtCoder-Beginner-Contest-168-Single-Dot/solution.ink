// Translated from solution.cpp.

var N: dynamic;

var M: dynamic;

var A = cpp_array(1000);

var B = cpp_array(1000);

var C = cpp_array(1000);

var D = cpp_array(1000);

var E = cpp_array(1000);

var F = cpp_array(1000);

var vis = cpp_array(3010, 3010);

var WX = cpp_array(3010);

var WY = cpp_array(3010);

var wx = cpp_array(3010, 3010);

var wy = cpp_array(3010, 3010);

func main()
{
  read(N, M);
  var X: dynamic;
  var Y: dynamic;
  X.push_back(cpp_cast(-2e9));
  X.push_back(0);
  X.push_back(cpp_cast(2e9));
  Y.push_back(cpp_cast(-2e9));
  Y.push_back(0);
  Y.push_back(cpp_cast(2e9));
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i], B[i], C[i]);
      X.push_back(A[i]);
      X.push_back(B[i]);
      Y.push_back(C[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      read(D[i], E[i], F[i]);
      X.push_back(D[i]);
      Y.push_back(E[i]);
      Y.push_back(F[i]);
      i += 1;
    }
  }
  sort(X.begin(), X.end());
  X.erase(unique(X.begin(), X.end()), X.end());
  sort(Y.begin(), Y.end());
  Y.erase(unique(Y.begin(), Y.end()), Y.end());
  {
    var i = 0;
    while ((i < N))
    {
      A[i] = (lower_bound(X.begin(), X.end(), A[i]) - X.begin());
      B[i] = (lower_bound(X.begin(), X.end(), B[i]) - X.begin());
      C[i] = (lower_bound(Y.begin(), Y.end(), C[i]) - Y.begin());
      WX[C[i]].push_back(make_pair(A[i], B[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      D[i] = (lower_bound(X.begin(), X.end(), D[i]) - X.begin());
      E[i] = (lower_bound(Y.begin(), Y.end(), E[i]) - Y.begin());
      F[i] = (lower_bound(Y.begin(), Y.end(), F[i]) - Y.begin());
      WY[D[i]].push_back(make_pair(E[i], F[i]));
      i += 1;
    }
  }
  WX[0].push_back(make_pair(0, (X.size() + 1)));
  WX[Y.size()].push_back(make_pair(0, (X.size() + 1)));
  {
    var i = 0;
    while ((i <= Y.size()))
    {
      sort(WX[i].begin(), WX[i].end());
      {
        var j = 0;
        while ((j < WX[i].size()))
        {
          var L = WX[i][j].first;
          var R = WX[i][j].second;
          while (((j < WX[i].size()) && (WX[i][j].first <= R)))
          {
            R = max(R, WX[i][j].second);
            j += 1;
          }
          {
            var k = L;
            while ((k < R))
            {
              wx[i][k] = true;
              k += 1;
            }
          }
        }
      }
      i += 1;
    }
  }
  WY[0].push_back(make_pair(0, (Y.size() + 1)));
  WY[X.size()].push_back(make_pair(0, (Y.size() + 1)));
  {
    var i = 0;
    while ((i <= X.size()))
    {
      sort(WY[i].begin(), WY[i].end());
      {
        var j = 0;
        while ((j < WY[i].size()))
        {
          var L = WY[i][j].first;
          var R = WY[i][j].second;
          while (((j < WY[i].size()) && (WY[i][j].first <= R)))
          {
            R = max(R, WY[i][j].second);
            j += 1;
          }
          {
            var k = L;
            while ((k < R))
            {
              wy[i][k] = true;
              k += 1;
            }
          }
        }
      }
      i += 1;
    }
  }
  var P: dynamic;
  var sx: dynamic;
  var sy: dynamic;
  {
    var id = (lower_bound(X.begin(), X.end(), 0) - X.begin());
    sx.push_back((id - 1));
    sx.push_back(id);
  }
  {
    var id = (lower_bound(Y.begin(), Y.end(), 0) - Y.begin());
    sy.push_back((id - 1));
    sy.push_back(id);
  }
  for (var x in sx)
  {
    for (var y in sy)
    {
      vis[x][y] = true;
      P.push(make_pair(x, y));
    }
  }
  while ((!P.empty()))
  {
    var x = P.front().first;
    var y = P.front().second;
    P.pop();
    if (((!wy[x][y]) && (!vis[(x - 1)][y])))
    {
      vis[(x - 1)][y] = true;
      P.push(make_pair((x - 1), y));
    }
    if (((!wx[y][x]) && (!vis[x][(y - 1)])))
    {
      vis[x][(y - 1)] = true;
      P.push(make_pair(x, (y - 1)));
    }
    if (((!wy[(x + 1)][y]) && (!vis[(x + 1)][y])))
    {
      vis[(x + 1)][y] = true;
      P.push(make_pair((x + 1), y));
    }
    if (((!wx[(y + 1)][x]) && (!vis[x][(y + 1)])))
    {
      vis[x][(y + 1)] = true;
      P.push(make_pair(x, (y + 1)));
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < X.size()))
    {
      {
        var j = 0;
        while ((j < Y.size()))
        {
          if ((!vis[i][j]))
          {
            j += 1;
            continue;
          }
          if (((((i == 0) || (j == 0)) || ((i + 1) == X.size())) || ((j + 1) == Y.size())))
          {
            write("INF", "\n");
            return 0;
          }
          ans += (((X[(i + 1)] - X[i])) * ((Y[(j + 1)] - Y[j])));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
