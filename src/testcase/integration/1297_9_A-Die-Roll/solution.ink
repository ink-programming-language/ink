// Translated from solution.cpp.

var y: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(y, w);
  var maxi: dynamic = max(y, w);
  var past: dynamic = (((6 - maxi)) + 1);
  var res: dynamic = ((past * 1.0) / 6);
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
