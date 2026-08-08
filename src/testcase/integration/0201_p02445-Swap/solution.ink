// Translated from solution.cpp.

var INF = (1 << 30);

var MAX = 10000;

var mod = 1000000007;

var pi = 3.141592653589;

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var q: dynamic;
  read(q);
  var l: dynamic;
  var r: dynamic;
  var t: dynamic;
  {
    var i = 0;
    while ((i < q))
    {
      read(l, r, t);
      swap_ranges((v.begin() + l), (v.begin() + r), (v.begin() + t));
      i += 1;
    }
  }
  printVec(v);
  return 0;
}

func printVec(vec: dynamic)
{
  {
    var i = 0;
    while ((i < vec.size()))
    {
      if (i)
      {
        write(cpp_char(" "));
      }
      write(vec[i]);
      i += 1;
    }
  }
  write(cpp_char("\n"));
}
