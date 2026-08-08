// Translated from solution.cpp.

var N = cpp_expression("//firstly");

var M = cpp_expression("//firstly");

var ll = dynamic;

var pll = cpp_expression("//firstly s");

var vll = cpp_expression("//firstly");

var vpll = cpp_expression("//firstly s");

var vvll = cpp_expression("//firstly s");

var endl = cpp_expression("//fi");

var umap = cpp_expression("//firstly save by ct");

var adj: dynamic;

var vis: dynamic;

var viss: dynamic;

var rnk: dynamic;

var parent: dynamic;

var sz: dynamic;

var sieve_max = cpp_expression("//first");

var spf = cpp_array((sieve_max + 1));

var mp: dynamic;

func yg(v: dynamic, l: dynamic, r: dynamic, prev: dynamic)
{
  if ((r < l))
  {
    return;
  }
  if ((r == l))
  {
    mp[r] = prev;
    return;
  }
  var mx = LLONG_MIN;
  var ct = 0;
  {
    var i = l;
    while ((i <= r))
    {
      if ((v[i] > mx))
      {
        mx = v[i];
        ct = i;
      }
      i += 1;
    }
  }
  mp[ct] = prev;
  yg(v, l, (ct - 1), (prev + 1));
  yg(v, (ct + 1), r, (prev + 1));
  return;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  var temp: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    mp.clear();
    var n: dynamic;
    read(n);
    var v: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        read(temp);
        v.push_back(temp);
        i += 1;
      }
    }
    yg(v, 0, (n - 1), 0);
    {
      var i = 0;
      while ((i < n))
      {
        write(mp[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
