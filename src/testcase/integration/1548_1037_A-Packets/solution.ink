// Translated from solution.cpp.

var N = 100001;

var v: dynamic;

func main()
{
  v.push_back(1);
  {
    var i = 1;
    while ((i < 31))
    {
      v.push_back((v[(i - 1)] * 2));
      i += 1;
    }
  }
  var n: dynamic;
  read(n);
  var pos = (upper_bound(v.begin(), v.end(), n) - v.begin());
  write(pos, "\n");
  return 0;
}
