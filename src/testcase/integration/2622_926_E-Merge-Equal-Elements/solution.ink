// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      a.push_back(x);
      i += 1;
    }
  }
  var it: dynamic = a.begin();
  while ((it != cpp_update(a.end(), "--")))
  {
    if (((*it) == (*next(it))))
    {
      a.erase(next(it));
      (*it) += 1;
      if ((it != a.begin()))
      {
        it -= 1;
      }
    } else
    {
      it += 1;
    }
  }
  write(a.size(), cpp_char("\n"));
  for (var x: dynamic in a)
  {
    write(x, cpp_char(" "));
  }
  write(cpp_char("\n"));
}
