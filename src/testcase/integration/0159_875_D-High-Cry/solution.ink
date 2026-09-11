// Translated from solution.cpp.

var N: dynamic = (2e5 + 5);

var q: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var go: dynamic = cpp_array(N);

class cell
{
  var max_l: dynamic = cpp_uninitialized();
  var max_r: dynamic = cpp_uninitialized();
  var or_l: dynamic = cpp_uninitialized();
  var or_r: dynamic = cpp_uninitialized();
}

var d: dynamic = cpp_array(35, N);

var b: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var q: dynamic = a[i];
      var k: dynamic = 1;
      while ((q != 0))
      {
        d[i][k] = (q % 2);
        q /= 2;
        k += 1;
      }
      i += 1;
    }
  }
  q.push([0, 2e9]);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      while ((q.top().second <= a[i]))
      {
        q.pop();
      }
      b[i].max_l = q.top().first;
      q.push([i, a[i]]);
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    q.pop();
  }
  q.push([(n + 1), 2e9]);
  {
    var i: dynamic = n;
    while ((i > 0))
    {
      while ((q.top().second < a[i]))
      {
        q.pop();
      }
      b[i].max_r = q.top().first;
      q.push([i, a[i]]);
      i -= 1;
    }
  }
  while ((!q.empty()))
  {
    q.pop();
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var k: dynamic = 0;
      {
        var j: dynamic = 1;
        while ((j < 35))
        {
          if ((d[i][j] == 0))
          {
            k = max(k, go[j]);
          } else
          {
            go[j] = i;
          }
          j += 1;
        }
      }
      b[i].or_l = k;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 35))
    {
      go[i] = (n + 1);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i > 0))
    {
      var k: dynamic = (n + 1);
      {
        var j: dynamic = 1;
        while ((j < 35))
        {
          if ((d[i][j] == 0))
          {
            k = min(k, go[j]);
          } else
          {
            go[j] = i;
          }
          j += 1;
        }
      }
      b[i].or_r = k;
      i -= 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((b[i].max_l >= b[i].or_l))
      {
        if ((b[i].max_r <= b[i].or_r))
        {
          i += 1;
          continue;
        } else
        {
          ans += (((i - b[i].max_l)) * ((b[i].max_r - b[i].or_r)));
        }
      } else if ((b[i].max_r <= b[i].or_r))
      {
        ans += (((b[i].or_l - b[i].max_l)) * ((b[i].max_r - i)));
      } else
      {
        ans += (((b[i].or_l - b[i].max_l)) * ((b[i].or_r - i)));
        ans += (((b[i].max_r - b[i].or_r)) * ((i - b[i].or_l)));
        ans += (((b[i].or_l - b[i].max_l)) * ((b[i].max_r - b[i].or_r)));
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
