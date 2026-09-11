// Translated from solution.cpp.

var N: dynamic = (2e5 + 10);

var mod: dynamic = 1000000007;

var A: dynamic = cpp_array(N);

var rmq: dynamic = cpp_array(25, N);

var getL: dynamic = cpp_array(N);

func go(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      rmq[i][0] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      getL[i] = log2(i);
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((((1 << j)) <= n))
    {
      {
        var i: dynamic = 0;
        while ((((i + ((1 << j))) - 1) < n))
        {
          if ((A[rmq[i][(j - 1)]] < A[rmq[(i + ((1 << ((j - 1)))))][(j - 1)]]))
          {
            rmq[i][j] = rmq[i][(j - 1)];
          } else
          {
            rmq[i][j] = rmq[(i + ((1 << ((j - 1)))))][(j - 1)];
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
}

func getmin(i: dynamic, j: dynamic) -> dynamic
{
  var k: dynamic = getL[((j - i) + 1)];
  if ((A[rmq[i][k]] < A[rmq[((j - ((1 << k))) + 1)][k]]))
  {
    return A[rmq[i][k]];
  }
  return A[rmq[((j - ((1 << k))) + 1)][k]];
}

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(d, n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      v.push_back(make_pair(x, y));
      i += 1;
    }
  }
  v.push_back(make_pair(d, 0));
  sort(v.begin(), v.end());
  var st: dynamic = 0;
  var fuel: dynamic = n;
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      A[i] = v[i].second;
      i += 1;
    }
  }
  go((m + 1));
  var cst: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      if (((v[i].first - st) > n))
      {
        cst = -1;
        break;
      }
      fuel -= ((v[i].first - st));
      var l: dynamic = (i + 1);
      var r: dynamic = (v.size() - 1);
      var res: dynamic = i;
      while ((l <= r))
      {
        var mid: dynamic = (((l + r)) >> 1);
        if (((v[mid].first - v[i].first) <= fuel))
        {
          l = (mid + 1);
          res = mid;
        } else
        {
          r = (mid - 1);
        }
      }
      var nxtmin: dynamic = i;
      l = (i + 1);
      r = (v.size() - 1);
      while ((l <= r))
      {
        var mid: dynamic = (((l + r)) >> 1);
        if ((getmin((i + 1), mid) <= v[i].second))
        {
          r = (mid - 1);
          nxtmin = mid;
        } else
        {
          l = (mid + 1);
        }
      }
      st = v[i].first;
      if ((res >= nxtmin))
      {
        i += 1;
        continue;
      }
      cst += ((((min(n, (v[nxtmin].first - v[i].first)) - fuel)) * 1) * v[i].second);
      fuel = min(n, (v[nxtmin].first - v[i].first));
      i += 1;
    }
  }
  write(cst);
  return 0;
}
