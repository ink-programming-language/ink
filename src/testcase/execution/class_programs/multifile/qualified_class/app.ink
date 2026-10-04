// Qualified class names survive independent object compilation and archive reload.
import provider;

func total(Value: provider.Point): i32
{
  var Local = Value;
  return Local.add(2);
}

func main(): i32
{
  return total(provider.echo(provider.Point()));
}
