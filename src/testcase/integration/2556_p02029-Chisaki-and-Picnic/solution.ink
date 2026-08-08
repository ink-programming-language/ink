// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(c[i], d[i]);
      i += 1;
    }
  }
  {
    var x: dynamic;
    var y: dynamic;
    x.emplace_back(c[0]);
    y.emplace_back(d[0]);
    {
      var i = 1;
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
  var pq: dynamic;
  var j = (n - 1);
  {
    var i = (m - 1);
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
  var ans = 0;
  while ((!pq.empty()))
  {
    ans += pq.top();
    pq.pop();
  }
  write(ans, "\n");
  return 0;
}
