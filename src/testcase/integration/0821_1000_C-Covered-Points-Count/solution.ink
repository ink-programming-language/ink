// Translated from solution.cpp.

var maxn: dynamic = (2e6 + 5);

var l: dynamic = cpp_array(maxn);

var r: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxn);

var tl: dynamic = cpp_array(maxn);

var tr: dynamic = cpp_array(maxn);

var s: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ds: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      tl[i] = (lower_bound(ds.begin(), ds.end(), l[i]) - ds.begin());
      tr[i] = (lower_bound(ds.begin(), ds.end(), r[i]) - ds.begin());
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      s[tl[i]] += 1;
      s[(tr[i] + 1)] -= 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      s[i] += s[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (ds.size() - 1)))
    {
      var nxt: dynamic = (ds[(i + 1)] - 1);
      ans[s[i]] += ((nxt - ds[i]) + 1);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
  return 0;
}
