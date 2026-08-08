// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var k: dynamic;
  var rest = 0;
  var ans = 0;
  var a: dynamic;
  read(n, k);
  a.resize(n);
  {
    var i = (0);
    while ((i < (n)))
    {
      read(a[i]);
      ans += (a[i] / 10);
      rest += (10 - (((a[i] + 9)) / 10));
      a[i] = (10 - (a[i] % 10));
      i += 1;
    }
  }
  sort((a).begin(), (a).end());
  {
    var i = (0);
    while ((i < (n)))
    {
      if (((a[i] == 10) || (k < a[i])))
      {
        break;
      }
      ans += 1;
      k -= a[i];
      i += 1;
    }
  }
  ans += min((k / 10), rest);
  write(ans, "\n");
  return 0;
}
