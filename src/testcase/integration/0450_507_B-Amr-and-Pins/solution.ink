// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var r: dynamic;
  var x: dynamic;
  var y: dynamic;
  var x1: dynamic;
  var y1: dynamic;
  read(r, x, y, x1, y1);
  var ans: dynamic;
  ans = ceil((sqrt((pow((x - x1), 2) + pow((y - y1), 2))) / ((2 * r))));
  write(ans);
  return 0;
}
