// Translated from solution.cpp.

func main() -> dynamic
{
  var list: dynamic = [[0, 300, 500, 600, 700, 1350, 1650], [6, 0, 350, 450, 600, 1150, 1500], [13, 7, 0, 250, 400, 1000, 1350], [18, 12, 5, 0, 250, 850, 1300], [23, 17, 10, 5, 0, 600, 1150], [43, 37, 30, 25, 20, 0, 500], [58, 52, 45, 40, 35, 15, 0]];
  var in_cpp: dynamic = cpp_uninitialized();
  var out: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var start: dynamic = cpp_uninitialized();
  var end: dynamic = cpp_uninitialized();
  var fee: dynamic = cpp_uninitialized();
  var half: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> in_cpp), in_cpp))
  {
    read(h, m);
    start = ((100 * h) + m);
    read(out, h, m);
    end = ((100 * h) + m);
    if ((in_cpp > out))
    {
      swap(in_cpp, out);
    }
    in_cpp -= 1;
    out -= 1;
    if (((((1730 <= start) && (start <= 1930))) || (((1730 <= end) && (end <= 1930)))))
    {
      half = true;
    } else
    {
      half = false;
    }
    if ((list[out][in_cpp] > 40))
    {
      half = false;
    }
    fee = list[in_cpp][out];
    if (half)
    {
      fee /= 2;
      fee += 25;
      fee /= 50;
      fee *= 50;
    }
    write(fee, "\n");
  }
}
