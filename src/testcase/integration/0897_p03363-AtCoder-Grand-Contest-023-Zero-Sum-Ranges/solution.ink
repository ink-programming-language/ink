// Translated from solution.cpp.

var MOD = (1e9 + 7);

func main()
{
  var n: dynamic;
  read(n);
  var m: dynamic;
  var ans = [];
  var sum = [];
  m[0] += 1;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      sum += a;
      m[sum] += 1;
      i += 1;
    }
  }
  for (var i in m)
  {
    ans += ((i.second * ((i.second - 1))) / 2);
  }
  write(ans, "\n");
}
