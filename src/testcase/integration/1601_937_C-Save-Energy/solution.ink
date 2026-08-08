// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var k: dynamic;
  var d: dynamic;
  var t: dynamic;
  read(k, d, t);
  if ((d < k))
  {
    d = (ceil((k / d)) * d);
  }
  var num = floor((((2 * t)) / ((d + k))));
  var time = (num * d);
  var left = (1 - (num * ((((d + k)) / ((2 * t))))));
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
