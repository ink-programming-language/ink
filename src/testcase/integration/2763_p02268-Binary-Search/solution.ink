// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_array(1001000);
  var q: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(S[i]);
      i += 1;
    }
  }
  read(q);
  {
    var i: dynamic = 0;
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
