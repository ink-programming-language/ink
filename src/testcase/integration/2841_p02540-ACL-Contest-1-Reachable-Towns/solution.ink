// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#includ");

var MOD: dynamic = cpp_expression("#include");

var INF: dynamic = cpp_expression("#include <bits/stdc");

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(xy[i].first.first, xy[i].first.second);
      xy[i].second = i;
      i += 1;
    }
  }
  sort(xy.begin(), xy.end());
  m[0] = xy[0].first.second;
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      m[i] = min(m[(i - 1)], xy[i].first.second);
      i += 1;
    }
  }
  M[(N - 1)] = xy[(N - 1)].first.second;
  {
    var i: dynamic = (N - 2);
    while ((i >= 0))
    {
      M[i] = max(M[(i + 1)], xy[i].first.second);
      i -= 1;
    }
  }
  var cnt: dynamic = cpp_construct(N, 1);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      if ((m[(i - 1)] < M[i]))
      {
        cnt[i] = (cnt[(i - 1)] + 1);
      }
      i += 1;
    }
  }
  var count: dynamic = cnt[(N - 1)];
  ans[xy[(N - 1)].second] = count;
  {
    var i: dynamic = (N - 2);
    while ((i >= 0))
    {
      if ((cnt[(i + 1)] == 1))
      {
        count = cnt[i];
      }
      ans[xy[i].second] = count;
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
}
