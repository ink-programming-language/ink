// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var S = cpp_array(1001000);
  var q: dynamic;
  var k: dynamic;
  var sum = 0;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(S[i]);
      i += 1;
    }
  }
  read(q);
  {
    var i = 0;
    while ((i < q))
    {
      read(k);
      if (((*lower_bound(S, (S + n), k)) == k))
      {
        sum += 1;
      }
      i += 1;
    }
  }
  write(sum, "\n");
}
