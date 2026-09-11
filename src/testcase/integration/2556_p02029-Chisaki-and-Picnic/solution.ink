// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(c[i], d[i]);
      i += 1;
    }
  }
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    x.emplace_back(c[0]);
    y.emplace_back(d[0]);
    {
      var i: dynamic = 1;
      while ((i < m))
      {
        if ((y.back() <= d[i]))
        {
          i += 1;
          continue;
        }
        if ((x.back() == c[i]))
        {
          y.back() = d[i];
        } else
        {
          x.emplace_back(c[i]);
          y.emplace_back(d[i]);
        }
        i += 1;
      }
    }
    m = x.size();
    c = x;
    d = y;
  }
  var pq: dynamic = cpp_uninitialized();
  var j: dynamic = (n - 1);
  {
    var i: dynamic = (m - 1);
    while ((i >= 0))
    {
      while (((j >= 0) && (a[j] >= c[i])))
      {
        pq.emplace(b[cpp_update(j, "--")]);
      }
      while ((cpp_cast(pq.size()) >= d[i]))
      {
        pq.pop();
      }
      i -= 1;
    }
  }
  while ((j >= 0))
  {
    pq.emplace(b[cpp_update(j, "--")]);
  }
  var ans: dynamic = 0;
  while ((!pq.empty()))
  {
    ans += pq.top();
    pq.pop();
  }
  write(ans, "\n");
  return 0;
}
