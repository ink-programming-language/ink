// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var A: dynamic;
  A.reserve(n);
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      A.push_back(make_pair(x, y));
      i += 1;
    }
  }
  sort(A.begin(), A.end());
  {
    var it = A.begin();
    while ((it != A.end()))
    {
      write(it->first, cpp_char(" "), it->second, "\n");
      it += 1;
    }
  }
}
