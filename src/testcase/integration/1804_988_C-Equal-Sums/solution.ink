// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  read(k);
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var n: dynamic = cpp_uninitialized();
      read(n);
      a[i].assign(n, 0);
      var sum: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          read(a[i][j]);
          sum += a[i][j];
          j += 1;
        }
      }
      s[i] = sum;
      i += 1;
    }
  }
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var m: dynamic = a[i].size();
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          var index: dynamic = (s[i] - a[i][j]);
          if ((c[index].find(i) != c[index].end()))
          {
            j += 1;
            continue;
          } else
          {
            c[index].insert(i);
          }
          b[index].push_back([(i + 1), (j + 1)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var it: dynamic = b.begin();
    while ((it != b.end()))
    {
      if (((it->second).size() >= 2))
      {
        write("YES\n");
        write(it->second[0].first, " ", it->second[0].second, "\n");
        write(it->second[1].first, " ", it->second[1].second, "\n");
        return 0;
      }
      it += 1;
    }
  }
  write("NO\n");
  return 0;
}
