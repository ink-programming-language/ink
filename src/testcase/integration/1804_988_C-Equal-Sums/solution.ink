// Translated from solution.cpp.

func main()
{
  var k: dynamic;
  read(k);
  var a: dynamic;
  {
    var i = 0;
    while ((i < k))
    {
      var n: dynamic;
      read(n);
      a[i].assign(n, 0);
      var sum = 0;
      {
        var j = 0;
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
  var b: dynamic;
  var c: dynamic;
  {
    var i = 0;
    while ((i < k))
    {
      var m = a[i].size();
      {
        var j = 0;
        while ((j < m))
        {
          var index = (s[i] - a[i][j]);
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
    var it = b.begin();
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
