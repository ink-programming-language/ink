// Translated from solution.cpp.

var maxn = (2e6 + 5);

var l = cpp_array(maxn);

var r = cpp_array(maxn);

var ans = cpp_array(maxn);

var tl = cpp_array(maxn);

var tr = cpp_array(maxn);

var s = cpp_array(maxn);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var ds: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(l[i], r[i]);
      ds.push_back(l[i]);
      ds.push_back(r[i]);
      ds.push_back((l[i] - 1));
      ds.push_back((l[i] + 1));
      ds.push_back((r[i] - 1));
      ds.push_back((r[i] + 1));
      i += 1;
    }
  }
  sort(ds.begin(), ds.end());
  ds.resize((unique(ds.begin(), ds.end()) - ds.begin()));
  {
    var i = 0;
    while ((i < n))
    {
      tl[i] = (lower_bound(ds.begin(), ds.end(), l[i]) - ds.begin());
      tr[i] = (lower_bound(ds.begin(), ds.end(), r[i]) - ds.begin());
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      s[tl[i]] += 1;
      s[(tr[i] + 1)] -= 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < maxn))
    {
      s[i] += s[(i - 1)];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (ds.size() - 1)))
    {
      var nxt = (ds[(i + 1)] - 1);
      ans[s[i]] += ((nxt - ds[i]) + 1);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
  return 0;
}
