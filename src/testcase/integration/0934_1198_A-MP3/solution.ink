// Translated from solution.cpp.

var N = (4e5 + 5);

var a = cpp_array(N);

func solve()
{
  var n: dynamic;
  var I: dynamic;
  read(n, I);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var k = (((8 * I)) / n);
  var K = pow(2, min(20, k));
  var v: dynamic;
  var psum: dynamic;
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var j = i;
      var cnt = 0;
      while (((j < n) && ((a[i] == a[j]))))
      {
        j += 1;
        cnt += 1;
      }
      v.push_back(a[i]);
      psum.push_back(cnt);
      i = (j - 1);
      i += 1;
    }
  }
  if ((K >= v.size()))
  {
    ans = 0;
  } else
  {
    var sz = v.size();
    {
      var i = 1;
      while ((i < sz))
      {
        psum[i] += psum[(i - 1)];
        i += 1;
      }
    }
    var i = 1;
    var j = K;
    ans = (psum[(sz - 1)] - psum[(K - 1)]);
    while ((j < sz))
    {
      var temp = (psum[(sz - 1)] - psum[j]);
      temp += psum[(i - 1)];
      ans = min(ans, temp);
      i += 1;
      j += 1;
    }
  }
  write(ans, "\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  solve();
}
