// Translated from solution.cpp.

var y: dynamic;

var w: dynamic;

func main()
{
  read(y, w);
  var maxi = max(y, w);
  var past = (((6 - maxi)) + 1);
  var res = ((past * 1.0) / 6);
  if ((res == 1))
  {
    write("1/1", "\n");
    return 0;
  }
  if ((past == 5))
  {
    write(past, "/6", "\n");
    return 0;
  }
  if ((past == 4))
  {
    write("2/3", "\n");
    return 0;
  }
  write((past / past), "/", (6 / past), "\n");
  return 0;
}
