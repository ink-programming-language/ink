// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var k: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(k, d, t);
  if ((d < k))
  {
    d = (ceil((k / d)) * d);
  }
  var num: dynamic = floor((((2 * t)) / ((d + k))));
  var time: dynamic = (num * d);
  var left: dynamic = (1 - (num * ((((d + k)) / ((2 * t))))));
  if ((left <= ((k / t))))
  {
    time += (left * t);
  } else
  {
    time += k;
    time += (((left - (k / t))) * ((2 * t)));
  }
  write(fixed, setprecision(10), time);
}
