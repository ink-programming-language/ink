// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  var n: dynamic;
  read(n);
  var a: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      a.push_back(x);
      i += 1;
    }
  }
  var it = a.begin();
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
  for (var x in a)
  {
    write(x, cpp_char(" "));
  }
  write(cpp_char("\n"));
}
